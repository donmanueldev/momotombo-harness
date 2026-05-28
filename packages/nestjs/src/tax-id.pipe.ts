import {
  ArgumentMetadata,
  BadRequestException,
  Injectable,
  PipeTransform
} from '@nestjs/common'
import { validateTaxId } from '@arev/tax-id-validator'

@Injectable()
export class TaxIdPipe implements PipeTransform {
  constructor(private readonly country: 'NI') {}

  transform(value: string, metadata: ArgumentMetadata) {
    const result = validateTaxId({
      country: this.country,
      value
    })

    if (!result.valid) {
      throw new BadRequestException({
        errors: result.errors
      })
    }

    return result.normalized
  }
}
